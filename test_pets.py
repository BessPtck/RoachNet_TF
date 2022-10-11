# this code is based on 
# https://pyimagesearch.com/2022/02/21/u-net-image-segmentation-in-keras/  author: Margaret Maynard-Reid
# which is based on "U-Net: Convolutional Networks for Biomedical Image Segmentation" by O Ronneberger, P Fischer and T Brox
# @article{Maynard-Reid_2022_U-Net,
#  author = {Margaret Maynard-Reid},
#  title = {{U-Net} Image Segmentation in Keras},
#  journal = {PyImageSearch},
#  year = {2022},
#  note = {https://pyimg.co/6m5br},


import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
import tensorflow_datasets as tfds
import numpy as np

#for stuff like drawing images
import matplotlib.pyplot as plt


def resize(input_image, input_mask):
    input_image = tf.image.resize(input_image, (128,128), method="nearest")
    input_mask = tf.image.resize(input_image, (128,128), method="nearest")
    return input_image, input_mask

def augment(input_image, input_mask):
    if tf.random.uniform(())>0.5:
        input_image=tf.image.flip_left_right(input_image)
        input_mask=tf.image.flip_left_right(input_mask)
    return input_image, input_mask

def normalize(input_image, input_mask):
    input_image = tf.cast(input_image, tf.float32)/255.0 #this casts the values of the input image to a float32 and then normalizes them to 1
    input_mask -= 1
    return input_image, input_mask

def load_image_train(datapoint):
    input_image=datapoint["image"]
    input_mask=datapoint["segmentation_mask"]
    input_image, input_mask = resize(input_image, input_mask)
    input_image, input_mask = augment(input_image, input_mask)
    input_image, input_mask = normalize(input_image, input_mask)
    return input_image, input_mask

def load_image_test(datapoint):
    input_image = datapoint["image"]
    input_mask = datapoint["segmentation_mask"]
    input_image, input_mask = resize(input_image, input_mask)
    input_image, input_mask = normalize(input_image, input_mask)
    return input_image, input_mask



def double_conv_block(x, n_filters):
    x=layers.Conv2D(n_filters, 3, padding="same", activation="relu", kernel_initializer="he_normal")(x) #sets up to add a 2D conv with n_filters(convolution scrapes) of 3x3 size to the model, relu and kernal_init arre used in training the conv filters
    x=layers.Conv2D(n_filters, 3, padding="same", activation="relu", kernel_initializer="he_normal")(x)
    return x

def downsample_block(x,n_filters):
    f=double_conv_block(x,n_filters)
    p=layers.MaxPool2D(2)(f) #takes the maximum in a 2x2 window, to reduce the size of the image goint through the anal
    p=layers.Dropout(0.3)(p)#randomly shuts down around 0.3 of the neurons at each training step by zeroing their outputs in this case perhaps doesn't train 30% of the convolution each cycle
    return f,p

def upsample_block(x, conv_features, n_filters):
    x=layers.Conv2DTranspose(n_filters, 3, 2, padding="same")(x) #this slides a filter around to expand the layer
    x=layers.concatenate([x,conv_features])#effectively attaches these two into a bigger tensor, this is probably the 'skip connection'
    x=layers.Dropout(0.3)(x)
    x = double_conv_block(x,n_filters)
    return x

def build_unet_model():
    inputs = layers.Input(shape=(128,128,3)) #128 by 128 in pixels 3 color channels 
    #going down 
    # the p's are passed through the model builder here, the numbers in the functions are the n_filters the number of 'scrapes' used by each convolution layer
    #f's are passed straight accross; they are the previous p just after the double_conv but not with the reduction
    f1, p1=downsample_block(inputs,64)
    f2, p2=downsample_block(p1,128)
    f3, p3=downsample_block(p2,256)
    f4, p4=downsample_block(p3,512)
    
    bottleneck=double_conv_block(p4,1024) #just straight double  convul on the now very reduced image with a lot (1024 per convul ) of filter scrapes

    #going up
    #the u's are passed into the model builder here to add to the expansion of the layers, it looks like their are more trainable convolution filters used in the expansion
    #the f's are concatinated (joined adjacent to the up-sample track) effectively bypassing the full 'U' and just passing through to their equivalent level in the up run
    u6=upsample_block(bottleneck,f4,512)
    u7=upsample_block(u6,f3,256)
    u8=upsample_block(u7,f2,128)
    u9=upsample_block(u8,f1, 64)

    outputs = layers.Conv2D(3,1,padding="same", activation="softmax")(u9) #3 final convol filters, of 1x1
    
    unet_model = tf.keras.Model(inputs,outputs, name="U-net")
    return unet_model

#extra stuff to display the data after it is loaded
def display(display_list):
    plt.figure(figsize=(15,15))
    title=["Input Image", "True Mask", "Predicted Mask"]
    for i in range(len(display_list)):
        plt.subplot(1,len(display_list),i+1)
        plt.title(title[i])
        plt.imshow(tf.keras.utils.array_to_img(display_list[i]))
        plt.axis("off")
    plt.show()

def renderTest(train_batches):
    sample_batch=next(iter(train_batches))
    random_index = np.random.choice(sample_batch[0],shape[0])
    sample_image, sample_mask = sample_batch[0][random_index],sample_batch[1][random_index]
    display([sample_image,sample_mask])

#extra stuff showing the results
def create_mask(pred_mask):
    pred_mask=tf.argmax(pred_mask,axis=-1)
    pred_mask=pred_mask[...,tf.newaxis]
    return pred_mask[0]

def show_predictions(dataset=None, num=1):
    if dataset:
        for image, mask in dataset.take(num):
            pred_mask=unet_model.predict(image)
            display([image[0], mask[0], create_mask(pred_mask)])


def Run_test():
    dataset, info = tfds.load('oxford_iiit_pet:3.*.*', with_info=True)
    train_dataset=dataset["train"].map(load_image_train, number_parallel_calls=tf.data.AUTOTUNE)
    test_dataset=dataset["test"].map(load_image_test, num_parallel_calls=tf.data.AUTOTUNE)

    BATCH_SIZE=64
    BUFFER_SIZE=1000

    train_batches = train_dataset.cache().shuffle(BUFFER_SIZE).batch(BATCH_SIZE).repeat()
    train_batches = train_batches.prefetch(buffer_size=tf.data.experimental.AUTOTUNE)
    validation_batches = test_dataset.take(3000).batch(BATCH_SIZE)
    test_batches = test_dataset.skip(3000).take(669).batch(BATCH_SIZE)

    #optional show images
    renderTest(train_batches)

    ###
    #now actually train the model
    #setup model
    unet_model = build_unet_model()
    unet_model.compile(optimizer=tf.keras.optimizers.Adam(),loss="sparse_categorical_crossentropy",metrics="accuracy")

    #train the model
    NUM_EPOCHS=20
    TRAIN_LENGTH = info.splits["train"].num_examples
    STEPS_PER_EPOCH = TRAIN_LENGTH // BATCH_SIZE

    VAL_SUBSPLITS = 5
    TEST_LENGTH = info.splits["test"].num_examples
    VALIDATION_STEPS=TEST_LENGTH//BATCH_SIZE//VAL_SUBSPLITS

    model_history = unet_model.fit(train_batches,epochs=NUM_EPOCHS,steps_per_epoch=STEPS_PER_EPOCH,validation_steps=VALIDATION_STEPS,validation_data=validation_batches)
    ###

    #run the u-net on some test data and show the results
    show_predictions(test_batches,1) #the 2nd is the number of test images to show